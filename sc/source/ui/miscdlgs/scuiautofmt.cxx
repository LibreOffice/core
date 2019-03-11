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

#undef SC_DLLIMPLEMENTATION

#include <vcl/button.hxx>
#include <vcl/dialog.hxx>
#include <vcl/lstbox.hxx>
#include <vcl/svapp.hxx>
#include <vcl/weld.hxx>
#include <sfx2/strings.hrc>
#include <sfx2/sfxresid.hxx>
#include <strings.hrc>
#include <global.hxx>
#include <globstr.hrc>
#include <autoform.hxx>
#include <strindlg.hxx>
#include <scuiautofmt.hxx>
#include <scresid.hxx>
#include <helpids.h>

// AutoFormat-Dialog:

ScAutoFormatDlg::ScAutoFormatDlg(weld::Window* pParent,
    ScAutoFormat* pAutoFormat,
    const ScAutoFormatData* pSelFormatData,
    const ScViewData *pViewData)
    : GenericDialogController(pParent, "modules/scalc/ui/autoformattable.ui", "AutoFormatTableDialog")
    , aStrTitle(ScResId(STR_ADD_AUTOFORMAT_TITLE))
    , aStrLabel(ScResId(STR_ADD_AUTOFORMAT_LABEL))
    , aStrClose(ScResId(STR_BTN_AUTOFORMAT_CLOSE))
    , aStrDelMsg(ScResId(STR_DEL_AUTOFORMAT_MSG))
    , aStrRename(ScResId(STR_RENAME_AUTOFORMAT_TITLE))
    , pFormat(pAutoFormat)
    , pSelFmtData(pSelFormatData)
    , nIndex(0)
    , bCoreDataChanged(false)
    , bFmtInserted(false)
    , m_xLbFormat(m_xBuilder->weld_tree_view("formatlb"))
    , m_xBtnOk(m_xBuilder->weld_button("ok"))
    , m_xBtnCancel(m_xBuilder->weld_button("cancel"))
    , m_xBtnAdd(m_xBuilder->weld_button("add"))
    , m_xBtnRemove(m_xBuilder->weld_button("remove"))
    , m_xBtnRename(m_xBuilder->weld_button("rename"))
    , m_xBtnNumFormat(m_xBuilder->weld_check_button("numformatcb"))
    , m_xBtnBorder(m_xBuilder->weld_check_button("bordercb"))
    , m_xBtnFont(m_xBuilder->weld_check_button("fontcb"))
    , m_xBtnPattern(m_xBuilder->weld_check_button("patterncb"))
    , m_xBtnAlignment(m_xBuilder->weld_check_button("alignmentcb"))
    , m_xBtnAdjust(m_xBuilder->weld_check_button("autofitcb"))
    , m_xWndPreview(new weld::CustomWeld(*m_xBuilder, "preview", m_aWndPreview))
{
    m_aWndPreview.DetectRTL(pViewData);

    const int nWidth = m_xLbFormat->get_approximate_digit_width() * 32;
    const int nHeight = m_xLbFormat->get_height_rows(8);
    m_xLbFormat->set_size_request(nWidth, nHeight);
    m_xWndPreview->set_size_request(nWidth, nHeight);

    Init();
    ScAutoFormat::iterator it = pFormat->begin();
    m_aWndPreview.NotifyChange(it->second.get());
}

ScAutoFormatDlg::~ScAutoFormatDlg()
{
}

void ScAutoFormatDlg::Init()
{
    m_xLbFormat->connect_changed( LINK( this, ScAutoFormatDlg, SelFmtHdl ) );
    m_xBtnNumFormat->connect_toggled( LINK( this, ScAutoFormatDlg, CheckHdl ) );
    m_xBtnBorder->connect_toggled( LINK( this, ScAutoFormatDlg, CheckHdl ) );
    m_xBtnFont->connect_toggled( LINK( this, ScAutoFormatDlg, CheckHdl ) );
    m_xBtnPattern->connect_toggled( LINK( this, ScAutoFormatDlg, CheckHdl ) );
    m_xBtnAlignment->connect_toggled( LINK( this, ScAutoFormatDlg, CheckHdl ) );
    m_xBtnAdjust->connect_toggled( LINK( this, ScAutoFormatDlg, CheckHdl ) );
    m_xBtnAdd->connect_clicked ( LINK( this, ScAutoFormatDlg, AddHdl ) );
    m_xBtnRemove->connect_clicked ( LINK( this, ScAutoFormatDlg, RemoveHdl ) );
    m_xBtnOk->connect_clicked ( LINK( this, ScAutoFormatDlg, CloseHdl ) );
    m_xBtnCancel->connect_clicked ( LINK( this, ScAutoFormatDlg, CloseHdl ) );
    m_xBtnRename->connect_clicked ( LINK( this, ScAutoFormatDlg, RenameHdl ) );
    m_xLbFormat->connect_row_activated( LINK( this, ScAutoFormatDlg, DblClkHdl ) );

    for (const auto& rEntry : *pFormat)
        m_xLbFormat->append_text(rEntry.second->GetName());

    if (pFormat->size() == 1)
        m_xBtnRemove->set_sensitive(false);

    m_xLbFormat->select(0);
    m_xBtnRename->set_sensitive(false);
    m_xBtnRemove->set_sensitive(false);

    nIndex = 0;
    UpdateChecks();

    if ( !pSelFmtData )
    {
        m_xBtnAdd->set_sensitive(false);
        m_xBtnRemove->set_sensitive(false);
        bFmtInserted = true;
    }
}

void ScAutoFormatDlg::UpdateChecks()
{
    const ScAutoFormatData* pData = pFormat->findByIndex(nIndex);

    m_xBtnNumFormat->set_active( pData->GetIncludeValueFormat() );
    m_xBtnBorder->set_active( pData->GetIncludeFrame() );
    m_xBtnFont->set_active( pData->GetIncludeFont() );
    m_xBtnPattern->set_active( pData->GetIncludeBackground() );
    m_xBtnAlignment->set_active( pData->GetIncludeJustify() );
    m_xBtnAdjust->set_active( pData->GetIncludeWidthHeight() );
}

// Handler:

IMPL_LINK(ScAutoFormatDlg, CloseHdl, weld::Button&, rBtn, void)
{
    if (&rBtn == m_xBtnOk.get() || &rBtn == m_xBtnCancel.get())
    {
        if ( bCoreDataChanged )
            ScGlobal::GetOrCreateAutoFormat()->Save();

        m_xDialog->response( (&rBtn == m_xBtnOk.get()) ? RET_OK : RET_CANCEL );
    }
}

IMPL_LINK_NOARG(ScAutoFormatDlg, DblClkHdl, weld::TreeView&, void)
{
    if ( bCoreDataChanged )
        ScGlobal::GetOrCreateAutoFormat()->Save();

    m_xDialog->response( RET_OK );
}

IMPL_LINK(ScAutoFormatDlg, CheckHdl, weld::ToggleButton&, rBtn, void)
{
    ScAutoFormatData* pData = pFormat->findByIndex(nIndex);
    bool bCheck = rBtn.get_active();

    if (&rBtn == m_xBtnNumFormat.get())
        pData->SetIncludeValueFormat( bCheck );
    else if (&rBtn == m_xBtnBorder.get())
        pData->SetIncludeFrame( bCheck );
    else if (&rBtn == m_xBtnFont.get())
        pData->SetIncludeFont( bCheck );
    else if (&rBtn == m_xBtnPattern.get())
        pData->SetIncludeBackground( bCheck );
    else if (&rBtn == m_xBtnAlignment.get())
        pData->SetIncludeJustify( bCheck );
    else if (&rBtn == m_xBtnAdjust.get())
        pData->SetIncludeWidthHeight( bCheck );

    if ( !bCoreDataChanged )
    {
        m_xBtnCancel->set_label(aStrClose);
        bCoreDataChanged = true;
    }

    m_aWndPreview.NotifyChange( pData );
}

IMPL_LINK_NOARG(ScAutoFormatDlg, AddHdl, weld::Button&, void)
{
    if ( !bFmtInserted && pSelFmtData )
    {
        OUString aStrStandard( SfxResId(STR_STANDARD) );
        OUString aFormatName;
        bool bOk = false;

        while ( !bOk )
        {
            ScStringInputDlg aDlg(m_xDialog.get(), aStrTitle, aStrLabel, aFormatName,
                                  HID_SC_ADD_AUTOFMT, HID_SC_AUTOFMT_NAME);

            if (aDlg.run() == RET_OK)
            {
                aFormatName = aDlg.GetInputString();

                if ( !aFormatName.isEmpty() && aFormatName != aStrStandard && pFormat->find(aFormatName) == pFormat->end() )
                {
                    std::unique_ptr<ScAutoFormatData> pNewData(
                        new ScAutoFormatData( *pSelFmtData ));

                    pNewData->SetName( aFormatName );
                    ScAutoFormat::iterator it = pFormat->insert(std::move(pNewData));
                    bFmtInserted = it != pFormat->end();

                    if ( bFmtInserted )
                    {
                        size_t nPos = std::distance(pFormat->begin(), it);
                        m_xLbFormat->insert_text(nPos, aFormatName);
                        m_xLbFormat->select_text( aFormatName );
                        m_xBtnAdd->set_sensitive(false);

                        if ( !bCoreDataChanged )
                        {
                            m_xBtnCancel->set_label( aStrClose );
                            bCoreDataChanged = true;
                        }

                        SelFmtHdl( *m_xLbFormat.get() );
                        bOk = true;
                    }
                }

                if ( !bFmtInserted )
                {
                    std::unique_ptr<weld::MessageDialog> xBox(Application::CreateMessageDialog(m_xDialog.get(),
                                VclMessageType::Error, VclButtonsType::OkCancel,
                                ScResId(STR_INVALID_AFNAME)));

                    sal_uInt16 nRet = xBox->run();

                    bOk = ( nRet == RET_CANCEL );
                }
            }
            else
                bOk = true;
        }
    }
}

IMPL_LINK_NOARG(ScAutoFormatDlg, RemoveHdl, weld::Button&, void)
{
    if ( (nIndex > 0) && (m_xLbFormat->n_children() > 0) )
    {
        OUString aMsg = aStrDelMsg.getToken( 0, '#' )
                      + m_xLbFormat->get_selected_text()
                      + aStrDelMsg.getToken( 1, '#' );

        std::unique_ptr<weld::MessageDialog> xQueryBox(Application::CreateMessageDialog(m_xDialog.get(),
                                                       VclMessageType::Question, VclButtonsType::YesNo,
                                                       aMsg));
        xQueryBox->set_default_response(RET_YES);

        if (RET_YES == xQueryBox->run())
        {
            m_xLbFormat->remove(nIndex);
            m_xLbFormat->select(nIndex-1);

            if ( nIndex-1 == 0 )
                m_xBtnRemove->set_sensitive(false);

            if ( !bCoreDataChanged )
            {
                m_xBtnCancel->set_label( aStrClose );
                bCoreDataChanged = true;
            }

            ScAutoFormat::iterator it = pFormat->begin();
            std::advance(it, nIndex);
            pFormat->erase(it);
            nIndex--;

            SelFmtHdl( *m_xLbFormat.get() );
        }
    }

    SelFmtHdl( *m_xLbFormat.get() );
}

IMPL_LINK_NOARG(ScAutoFormatDlg, RenameHdl, weld::Button&, void)
{
    bool bOk = false;
    while( !bOk )
    {

        OUString aFormatName = m_xLbFormat->get_selected_text();
        OUString aEntry;

        ScStringInputDlg aDlg(m_xDialog.get(), aStrRename, aStrLabel, aFormatName,
                              HID_SC_REN_AFMT_DLG, HID_SC_REN_AFMT_NAME);
        if (aDlg.run() == RET_OK)
        {
            bool bFmtRenamed = false;
            aFormatName = aDlg.GetInputString();

            if (!aFormatName.isEmpty())
            {
                ScAutoFormat::iterator it = pFormat->begin(), itEnd = pFormat->end();
                for (; it != itEnd; ++it)
                {
                    aEntry = it->second->GetName();
                    if (aFormatName == aEntry)
                        break;
                }
                if (it == itEnd)
                {
                    // no format with this name yet, so we can rename

                    m_xLbFormat->remove(nIndex);
                    const ScAutoFormatData* p = pFormat->findByIndex(nIndex);
                    std::unique_ptr<ScAutoFormatData> pNewData(new ScAutoFormatData(*p));

                    it = pFormat->begin();
                    std::advance(it, nIndex);
                    pFormat->erase(it);

                    pNewData->SetName( aFormatName );

                    pFormat->insert(std::move(pNewData));

                    m_xLbFormat->freeze();
                    m_xLbFormat->clear();
                    for (it = pFormat->begin(); it != itEnd; ++it)
                    {
                        aEntry = it->second->GetName();
                        m_xLbFormat->append_text(aEntry);
                    }

                    m_xLbFormat->thaw();
                    m_xLbFormat->select_text(aFormatName);

                    if ( !bCoreDataChanged )
                    {
                        m_xBtnCancel->set_label( aStrClose );
                        bCoreDataChanged = true;
                    }

                    SelFmtHdl( *m_xLbFormat.get() );
                    bOk = true;
                    bFmtRenamed = true;
                }
            }
            if( !bFmtRenamed )
            {
                std::unique_ptr<weld::MessageDialog> xBox(Application::CreateMessageDialog(m_xDialog.get(),
                            VclMessageType::Error, VclButtonsType::OkCancel,
                            ScResId(STR_INVALID_AFNAME)));

                bOk = RET_CANCEL == xBox->run();
            }
        }
        else
            bOk = true;
    }
}

IMPL_LINK_NOARG(ScAutoFormatDlg, SelFmtHdl, weld::TreeView&, void)
{
    nIndex = m_xLbFormat->get_selected_index();
    UpdateChecks();

    if ( nIndex == 0 )
    {
        m_xBtnRename->set_sensitive(false);
        m_xBtnRemove->set_sensitive(false);
    }
    else
    {
        m_xBtnRename->set_sensitive(true);
        m_xBtnRemove->set_sensitive(true);
    }

    ScAutoFormatData* p = pFormat->findByIndex(nIndex);
    m_aWndPreview.NotifyChange(p);
}

OUString ScAutoFormatDlg::GetCurrFormatName()
{
    const ScAutoFormatData* p = pFormat->findByIndex(nIndex);
    return p ? p->GetName() : OUString();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
