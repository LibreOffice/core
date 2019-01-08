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

#include <scitems.hxx>
#include <svx/algitem.hxx>
#include <editeng/boxitem.hxx>
#include <editeng/brushitem.hxx>
#include <editeng/contouritem.hxx>
#include <editeng/colritem.hxx>
#include <editeng/crossedoutitem.hxx>
#include <editeng/fontitem.hxx>
#include <editeng/postitem.hxx>
#include <editeng/shdditem.hxx>
#include <editeng/udlnitem.hxx>
#include <editeng/wghtitem.hxx>
#include <svl/zforlist.hxx>
#include <vcl/lstbox.hxx>
#include <vcl/svapp.hxx>
#include <vcl/weld.hxx>
#include <sfx2/strings.hrc>
#include <sfx2/sfxresid.hxx>
#include <strings.hrc>
#include <scmod.hxx>
#include <attrib.hxx>
#include <zforauto.hxx>
#include <global.hxx>
#include <globstr.hrc>
#include <autoform.hxx>
#include <strindlg.hxx>
#include <scuiautofmt.hxx>
#include <scresid.hxx>
#include <document.hxx>
#include <helpids.h>

// AutoFormat-Dialog:

ScAutoFormatDlg::ScAutoFormatDlg(vcl::Window* pParent,
    ScAutoFormat* pAutoFormat,
    const ScAutoFormatData* pSelFormatData,
    const ScViewData *pViewData)
    : ModalDialog(pParent, "AutoFormatTableDialog",
        "modules/scalc/ui/autoformattable.ui")
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
{
    get(m_pLbFormat, "formatlb");
    get(m_pWndPreview, "preview");
    m_pWndPreview->DetectRTL(pViewData);
    get(m_pBtnOk, "ok");
    get(m_pBtnCancel, "cancel");
    get(m_pBtnAdd, "add");
    get(m_pBtnRemove, "remove");
    get(m_pBtnRename, "rename");
    get(m_pBtnNumFormat, "numformatcb");
    get(m_pBtnBorder, "bordercb");
    get(m_pBtnFont, "fontcb");
    get(m_pBtnPattern, "patterncb");
    get(m_pBtnAlignment, "alignmentcb");
    get(m_pBtnAdjust, "autofitcb");

    Init();
    ScAutoFormat::iterator it = pFormat->begin();
    m_pWndPreview->NotifyChange(it->second.get());
}

ScAutoFormatDlg::~ScAutoFormatDlg()
{
    disposeOnce();
}

void ScAutoFormatDlg::dispose()
{
    m_pLbFormat.clear();
    m_pWndPreview.clear();
    m_pBtnOk.clear();
    m_pBtnCancel.clear();
    m_pBtnAdd.clear();
    m_pBtnRemove.clear();
    m_pBtnRename.clear();
    m_pBtnNumFormat.clear();
    m_pBtnBorder.clear();
    m_pBtnFont.clear();
    m_pBtnPattern.clear();
    m_pBtnAlignment.clear();
    m_pBtnAdjust.clear();
    ModalDialog::dispose();
}

void ScAutoFormatDlg::Init()
{
    m_pLbFormat->SetSelectHdl( LINK( this, ScAutoFormatDlg, SelFmtHdl ) );
    m_pBtnNumFormat->SetClickHdl ( LINK( this, ScAutoFormatDlg, CheckHdl ) );
    m_pBtnBorder->SetClickHdl ( LINK( this, ScAutoFormatDlg, CheckHdl ) );
    m_pBtnFont->SetClickHdl ( LINK( this, ScAutoFormatDlg, CheckHdl ) );
    m_pBtnPattern->SetClickHdl ( LINK( this, ScAutoFormatDlg, CheckHdl ) );
    m_pBtnAlignment->SetClickHdl ( LINK( this, ScAutoFormatDlg, CheckHdl ) );
    m_pBtnAdjust->SetClickHdl ( LINK( this, ScAutoFormatDlg, CheckHdl ) );
    m_pBtnAdd->SetClickHdl ( LINK( this, ScAutoFormatDlg, AddHdl ) );
    m_pBtnRemove->SetClickHdl ( LINK( this, ScAutoFormatDlg, RemoveHdl ) );
    m_pBtnOk->SetClickHdl ( LINK( this, ScAutoFormatDlg, CloseHdl ) );
    m_pBtnCancel->SetClickHdl ( LINK( this, ScAutoFormatDlg, CloseHdl ) );
    m_pBtnRename->SetClickHdl ( LINK( this, ScAutoFormatDlg, RenameHdl ) );
    m_pLbFormat->SetDoubleClickHdl( LINK( this, ScAutoFormatDlg, DblClkHdl ) );

    for (const auto& rEntry : *pFormat)
        m_pLbFormat->InsertEntry(rEntry.second->GetName());

    if (pFormat->size() == 1)
        m_pBtnRemove->Disable();

    m_pLbFormat->SelectEntryPos( 0 );
    m_pBtnRename->Disable();
    m_pBtnRemove->Disable();

    nIndex = 0;
    UpdateChecks();

    if ( !pSelFmtData )
    {
        m_pBtnAdd->Disable();
        m_pBtnRemove->Disable();
        bFmtInserted = true;
    }
}

void ScAutoFormatDlg::UpdateChecks()
{
    const ScAutoFormatData* pData = pFormat->findByIndex(nIndex);

    m_pBtnNumFormat->Check( pData->GetIncludeValueFormat() );
    m_pBtnBorder->Check( pData->GetIncludeFrame() );
    m_pBtnFont->Check( pData->GetIncludeFont() );
    m_pBtnPattern->Check( pData->GetIncludeBackground() );
    m_pBtnAlignment->Check( pData->GetIncludeJustify() );
    m_pBtnAdjust->Check( pData->GetIncludeWidthHeight() );
}

// Handler:

IMPL_LINK( ScAutoFormatDlg, CloseHdl, Button *, pBtn, void )
{
    if (pBtn == m_pBtnOk || pBtn == m_pBtnCancel)
    {
        if ( bCoreDataChanged )
            ScGlobal::GetOrCreateAutoFormat()->Save();

        EndDialog( (pBtn == m_pBtnOk) ? RET_OK : RET_CANCEL );
    }
}

IMPL_LINK_NOARG(ScAutoFormatDlg, DblClkHdl, ListBox&, void)
{
    if ( bCoreDataChanged )
        ScGlobal::GetOrCreateAutoFormat()->Save();

    EndDialog( RET_OK );
}

IMPL_LINK( ScAutoFormatDlg, CheckHdl, Button *, pBtn, void )
{
    ScAutoFormatData* pData = pFormat->findByIndex(nIndex);
    bool bCheck = static_cast<CheckBox*>(pBtn)->IsChecked();

    if ( pBtn == m_pBtnNumFormat )
        pData->SetIncludeValueFormat( bCheck );
    else if ( pBtn == m_pBtnBorder )
        pData->SetIncludeFrame( bCheck );
    else if ( pBtn == m_pBtnFont )
        pData->SetIncludeFont( bCheck );
    else if ( pBtn == m_pBtnPattern )
        pData->SetIncludeBackground( bCheck );
    else if ( pBtn == m_pBtnAlignment )
        pData->SetIncludeJustify( bCheck );
    else if ( pBtn == m_pBtnAdjust )
        pData->SetIncludeWidthHeight( bCheck );

    if ( !bCoreDataChanged )
    {
        m_pBtnCancel->SetText( aStrClose );
        bCoreDataChanged = true;
    }

    m_pWndPreview->NotifyChange( pData );
}

IMPL_LINK_NOARG(ScAutoFormatDlg, AddHdl, Button*, void)
{
    if ( !bFmtInserted && pSelFmtData )
    {
        OUString aStrStandard( SfxResId(STR_STANDARD) );
        OUString aFormatName;
        bool bOk = false;

        while ( !bOk )
        {
            ScStringInputDlg aDlg(GetFrameWeld(), aStrTitle, aStrLabel, aFormatName,
                                  HID_SC_ADD_AUTOFMT, HID_SC_AUTOFMT_NAME);

            if (aDlg.run() == RET_OK)
            {
                aFormatName = aDlg.GetInputString();

                if ( !aFormatName.isEmpty() && aFormatName != aStrStandard && pFormat->find(aFormatName) == pFormat->end() )
                {
                    ScAutoFormatData* pNewData
                        = new ScAutoFormatData( *pSelFmtData );

                    pNewData->SetName( aFormatName );
                    bFmtInserted = pFormat->insert(pNewData);

                    if ( bFmtInserted )
                    {
                        ScAutoFormat::const_iterator it = pFormat->find(pNewData);
                        ScAutoFormat::const_iterator itBeg = pFormat->begin();
                        size_t nPos = std::distance(itBeg, it);
                        m_pLbFormat->InsertEntry(aFormatName, nPos);
                        m_pLbFormat->SelectEntry( aFormatName );
                        m_pBtnAdd->Disable();

                        if ( !bCoreDataChanged )
                        {
                            m_pBtnCancel->SetText( aStrClose );
                            bCoreDataChanged = true;
                        }

                        SelFmtHdl( *m_pLbFormat.get() );
                        bOk = true;
                    }
                    else
                        delete pNewData;

                }

                if ( !bFmtInserted )
                {
                    std::unique_ptr<weld::MessageDialog> xBox(Application::CreateMessageDialog(GetFrameWeld(),
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

IMPL_LINK_NOARG(ScAutoFormatDlg, RemoveHdl, Button*, void)
{
    if ( (nIndex > 0) && (m_pLbFormat->GetEntryCount() > 0) )
    {
        OUString aMsg = aStrDelMsg.getToken( 0, '#' )
                      + m_pLbFormat->GetSelectedEntry()
                      + aStrDelMsg.getToken( 1, '#' );

        std::unique_ptr<weld::MessageDialog> xQueryBox(Application::CreateMessageDialog(GetFrameWeld(),
                                                       VclMessageType::Question, VclButtonsType::YesNo,
                                                       aMsg));
        xQueryBox->set_default_response(RET_YES);

        if (RET_YES == xQueryBox->run())
        {
            m_pLbFormat->RemoveEntry( nIndex );
            m_pLbFormat->SelectEntryPos( nIndex-1 );

            if ( nIndex-1 == 0 )
                m_pBtnRemove->Disable();

            if ( !bCoreDataChanged )
            {
                m_pBtnCancel->SetText( aStrClose );
                bCoreDataChanged = true;
            }

            ScAutoFormat::iterator it = pFormat->begin();
            std::advance(it, nIndex);
            pFormat->erase(it);
            nIndex--;

            SelFmtHdl( *m_pLbFormat.get() );
        }
    }

    SelFmtHdl( *m_pLbFormat.get() );
}

IMPL_LINK_NOARG(ScAutoFormatDlg, RenameHdl, Button*, void)
{
    bool bOk = false;
    while( !bOk )
    {

        OUString aFormatName = m_pLbFormat->GetSelectedEntry();
        OUString aEntry;

        ScStringInputDlg aDlg(GetFrameWeld(), aStrRename, aStrLabel, aFormatName,
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

                    m_pLbFormat->RemoveEntry(nIndex );
                    const ScAutoFormatData* p = pFormat->findByIndex(nIndex);
                    ScAutoFormatData* pNewData
                        = new ScAutoFormatData(*p);

                    it = pFormat->begin();
                    std::advance(it, nIndex);
                    pFormat->erase(it);

                    pNewData->SetName( aFormatName );

                    pFormat->insert(pNewData);

                    m_pLbFormat->SetUpdateMode(false);
                    m_pLbFormat->Clear();
                    for (it = pFormat->begin(); it != itEnd; ++it)
                    {
                        aEntry = it->second->GetName();
                        m_pLbFormat->InsertEntry( aEntry );
                    }

                    m_pLbFormat->SetUpdateMode(true);
                    m_pLbFormat->SelectEntry( aFormatName);

                    if ( !bCoreDataChanged )
                    {
                        m_pBtnCancel->SetText( aStrClose );
                        bCoreDataChanged = true;
                    }

                    SelFmtHdl( *m_pLbFormat.get() );
                    bOk = true;
                    bFmtRenamed = true;
                }
            }
            if( !bFmtRenamed )
            {
                std::unique_ptr<weld::MessageDialog> xBox(Application::CreateMessageDialog(GetFrameWeld(),
                            VclMessageType::Error, VclButtonsType::OkCancel,
                            ScResId(STR_INVALID_AFNAME)));

                bOk = RET_CANCEL == xBox->run();
            }
        }
        else
            bOk = true;
    }
}

IMPL_LINK_NOARG(ScAutoFormatDlg, SelFmtHdl, ListBox&, void)
{
    nIndex = m_pLbFormat->GetSelectedEntryPos();
    UpdateChecks();

    if ( nIndex == 0 )
    {
        m_pBtnRename->Disable();
        m_pBtnRemove->Disable();
    }
    else
    {
        m_pBtnRename->Enable();
        m_pBtnRemove->Enable();
    }

    ScAutoFormatData* p = pFormat->findByIndex(nIndex);
    m_pWndPreview->NotifyChange(p);
}

OUString ScAutoFormatDlg::GetCurrFormatName()
{
    const ScAutoFormatData* p = pFormat->findByIndex(nIndex);
    return p ? p->GetName() : OUString();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
