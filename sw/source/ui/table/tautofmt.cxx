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
#include <vcl/weld.hxx>
#include <strings.hrc>
#include <view.hxx>
#include <shellres.hxx>
#include <tautofmt.hxx>

class SwStringInputDlg
{
private:
    std::unique_ptr<weld::Builder> m_xBuilder;
    std::unique_ptr<weld::Dialog> m_xDialog;
    std::unique_ptr<weld::Label> m_xLabel;
    std::unique_ptr<weld::Entry> m_xEdInput; // Edit obtains the focus.

public:
    SwStringInputDlg(weld::Window* pParent, const OUString& rTitle,
        const OUString& rEditTitle, const OUString& rDefault)
        : m_xBuilder(Application::CreateBuilder(pParent, "modules/swriter/ui/stringinput.ui"))
        , m_xDialog(m_xBuilder->weld_dialog("StringInputDialog"))
        , m_xLabel(m_xBuilder->weld_label("name"))
        , m_xEdInput(m_xBuilder->weld_entry("edit"))
    {
        m_xLabel->set_label(rEditTitle);
        m_xDialog->set_title(rTitle);
        m_xEdInput->set_text(rDefault);
        m_xEdInput->select_region(0, -1);
    }

    OUString GetInputString() const
    {
        return m_xEdInput->get_text();
    }

    short run() {  return m_xDialog->run(); }
};

// AutoFormat-Dialogue:
SwAutoFormatDlg::SwAutoFormatDlg( vcl::Window* pParent, SwWrtShell* pWrtShell,
                    bool bAutoFormat, const SwTableAutoFormat* pSelFormat )
    : SfxModalDialog(pParent, "AutoFormatTableDialog", "modules/swriter/ui/autoformattable.ui")
    , aStrTitle(SwResId(STR_ADD_AUTOFORMAT_TITLE))
    , aStrLabel(SwResId(STR_ADD_AUTOFORMAT_LABEL))
    , aStrClose(SwResId(STR_BTN_AUTOFORMAT_CLOSE))
    , aStrDelTitle(SwResId(STR_DEL_AUTOFORMAT_TITLE))
    , aStrDelMsg(SwResId(STR_DEL_AUTOFORMAT_MSG))
    , aStrRenameTitle(SwResId(STR_RENAME_AUTOFORMAT_TITLE))
    , aStrInvalidFormat(SwResId(STR_INVALID_AUTOFORMAT_NAME))
    , pShell(pWrtShell)
    , nIndex(0)
    , nDfltStylePos(0)
    , bCoreDataChanged(false)
    , bSetAutoFormat(bAutoFormat)
{
    get(m_pLbFormat, "formatlb");
    get(m_pFormatting, "formatting");
    get(m_pBtnNumFormat, "numformatcb");
    get(m_pBtnBorder, "bordercb");
    get(m_pBtnFont, "fontcb");
    get(m_pBtnPattern, "patterncb");
    get(m_pBtnAlignment, "alignmentcb");
    get(m_pBtnOk, "ok");
    get(m_pBtnCancel, "cancel");
    get(m_pBtnAdd, "add");
    get(m_pBtnRemove, "remove");
    get(m_pBtnRename, "rename");
    get(m_pWndPreview, "preview");

    m_pWndPreview->DetectRTL(pWrtShell);

    pTableTable = new SwTableAutoFormatTable;
    pTableTable->Load();

    Init(pSelFormat);
}

SwAutoFormatDlg::~SwAutoFormatDlg()
{
    disposeOnce();
}

void SwAutoFormatDlg::dispose()
{
    if (bCoreDataChanged)
        pTableTable->Save();
    delete pTableTable;
    m_pLbFormat.clear();
    m_pFormatting.clear();
    m_pBtnNumFormat.clear();
    m_pBtnBorder.clear();
    m_pBtnFont.clear();
    m_pBtnPattern.clear();
    m_pBtnAlignment.clear();
    m_pBtnOk.clear();
    m_pBtnCancel.clear();
    m_pBtnAdd.clear();
    m_pBtnRemove.clear();
    m_pBtnRename.clear();
    m_pWndPreview.clear();
    SfxModalDialog::dispose();
}

void SwAutoFormatDlg::Init( const SwTableAutoFormat* pSelFormat )
{
    Link<Button*,void> aLk( LINK( this, SwAutoFormatDlg, CheckHdl ) );
    m_pBtnBorder->SetClickHdl( aLk );
    m_pBtnFont->SetClickHdl( aLk );
    m_pBtnPattern->SetClickHdl( aLk );
    m_pBtnAlignment->SetClickHdl( aLk );
    m_pBtnNumFormat->SetClickHdl( aLk );

    m_pBtnAdd->SetClickHdl ( LINK( this, SwAutoFormatDlg, AddHdl ) );
    m_pBtnRemove->SetClickHdl ( LINK( this, SwAutoFormatDlg, RemoveHdl ) );
    m_pBtnRename->SetClickHdl ( LINK( this, SwAutoFormatDlg, RenameHdl ) );
    m_pBtnOk->SetClickHdl ( LINK( this, SwAutoFormatDlg, OkHdl ) );
    m_pLbFormat->SetSelectHdl( LINK( this, SwAutoFormatDlg, SelFormatHdl ) );

    m_pBtnAdd->Enable( bSetAutoFormat );

    nIndex = 0;
    if( !bSetAutoFormat )
    {
        // Then the list to be expanded by the entry "- none -".
        m_pLbFormat->InsertEntry( SwViewShell::GetShellRes()->aStrNone );
        nDfltStylePos = 1;
        nIndex = 255;
    }

    for (sal_uInt8 i = 0, nCount = static_cast<sal_uInt8>(pTableTable->size());
            i < nCount; i++)
    {
        SwTableAutoFormat const& rFormat = (*pTableTable)[ i ];
        m_pLbFormat->InsertEntry(rFormat.GetName());
        if (pSelFormat && rFormat.GetName() == pSelFormat->GetName())
            nIndex = i;
    }

    m_pLbFormat->SelectEntryPos( 255 != nIndex ? (nDfltStylePos + nIndex) : 0 );
    SelFormatHdl( *m_pLbFormat );
}

void SwAutoFormatDlg::UpdateChecks( const SwTableAutoFormat& rFormat, bool bEnable )
{
    m_pBtnNumFormat->Enable( bEnable );
    m_pBtnNumFormat->Check( rFormat.IsValueFormat() );

    m_pBtnBorder->Enable( bEnable );
    m_pBtnBorder->Check( rFormat.IsFrame() );

    m_pBtnFont->Enable( bEnable );
    m_pBtnFont->Check( rFormat.IsFont() );

    m_pBtnPattern->Enable( bEnable );
    m_pBtnPattern->Check( rFormat.IsBackground() );

    m_pBtnAlignment->Enable( bEnable );
    m_pBtnAlignment->Check( rFormat.IsJustify() );
}

static void lcl_SetProperties( SwTableAutoFormat* pTableAutoFormat, bool bVal )
{
    pTableAutoFormat->SetFont( bVal );
    pTableAutoFormat->SetJustify( bVal );
    pTableAutoFormat->SetFrame( bVal );
    pTableAutoFormat->SetBackground( bVal );
    pTableAutoFormat->SetValueFormat( bVal );
    pTableAutoFormat->SetWidthHeight( bVal );
}

void SwAutoFormatDlg::FillAutoFormatOfIndex( SwTableAutoFormat*& rToFill ) const
{
    if( 255 != nIndex )
    {
        if( rToFill )
            *rToFill = (*pTableTable)[ nIndex ];
        else
            rToFill = new SwTableAutoFormat( (*pTableTable)[ nIndex ] );
    }
    else
    {
        delete rToFill;
        rToFill = new SwTableAutoFormat( SwViewShell::GetShellRes()->aStrNone );
        lcl_SetProperties( rToFill, false );
    }
}

// Handler:

IMPL_LINK( SwAutoFormatDlg, CheckHdl, Button *, pBtn, void )
{
    SwTableAutoFormat* pData  = &(*pTableTable)[nIndex];
    bool bCheck = static_cast<CheckBox*>(pBtn)->IsChecked(), bDataChgd = true;

    if( pBtn == m_pBtnNumFormat )
        pData->SetValueFormat( bCheck );
    else if ( pBtn == m_pBtnBorder )
        pData->SetFrame( bCheck );
    else if ( pBtn == m_pBtnFont )
        pData->SetFont( bCheck );
    else if ( pBtn == m_pBtnPattern )
        pData->SetBackground( bCheck );
    else if ( pBtn == m_pBtnAlignment )
        pData->SetJustify( bCheck );
    else
        bDataChgd = false;

    if( bDataChgd )
    {
        if( !bCoreDataChanged )
        {
            m_pBtnCancel->SetText( aStrClose );
            bCoreDataChanged = true;
        }

        m_pWndPreview->NotifyChange( *pData );
    }
}

IMPL_LINK_NOARG(SwAutoFormatDlg, AddHdl, Button*, void)
{
    bool bOk = false, bFormatInserted = false;
    while( !bOk )
    {
        SwStringInputDlg aDlg(GetFrameWeld(), aStrTitle, aStrLabel, OUString());
        if (RET_OK == aDlg.run())
        {
            const OUString aFormatName(aDlg.GetInputString());

            if ( !aFormatName.isEmpty() )
            {
                size_t n;
                for( n = 0; n < pTableTable->size(); ++n )
                    if( (*pTableTable)[n].GetName() == aFormatName )
                        break;

                if( n >= pTableTable->size() )
                {
                    // Format with the name does not already exist, so take up.
                    std::unique_ptr<SwTableAutoFormat> pNewData(
                            new SwTableAutoFormat(aFormatName));
                    bool bGetOk = pShell->GetTableAutoFormat( *pNewData );
                    SAL_WARN_IF(!bGetOk, "sw.ui", "GetTableAutoFormat failed for: " << aFormatName);

                    // Insert sorted!!
                    for( n = 1; n < pTableTable->size(); ++n )
                        if( (*pTableTable)[ n ].GetName() > aFormatName )
                            break;

                    pTableTable->InsertAutoFormat(n, std::move(pNewData));
                    m_pLbFormat->InsertEntry( aFormatName, nDfltStylePos + n );
                    m_pLbFormat->SelectEntryPos( nDfltStylePos + n );
                    bFormatInserted = true;
                    m_pBtnAdd->Enable( false );
                    if ( !bCoreDataChanged )
                    {
                        m_pBtnCancel->SetText( aStrClose );
                        bCoreDataChanged = true;
                    }

                    SelFormatHdl( *m_pLbFormat );
                    bOk = true;
                }
            }

            if( !bFormatInserted )
            {
                std::unique_ptr<weld::MessageDialog> xBox(Application::CreateMessageDialog(GetFrameWeld(), VclMessageType::Error, VclButtonsType::OkCancel, aStrInvalidFormat));
                bOk = RET_CANCEL == xBox->run();
            }
        }
        else
            bOk = true;
    }
}

IMPL_LINK_NOARG(SwAutoFormatDlg, RemoveHdl, Button*, void)
{
    OUString aMessage = aStrDelMsg;
    aMessage += "\n\n";
    aMessage += m_pLbFormat->GetSelectedEntry();
    aMessage += "\n";

    std::unique_ptr<weld::MessageDialog> xBox(Application::CreateMessageDialog(GetFrameWeld(), VclMessageType::Question,
                                              VclButtonsType::OkCancel, aStrDelTitle));
    xBox->set_secondary_text(aMessage);

    if (xBox->run() == RET_OK)
    {
        m_pLbFormat->RemoveEntry( nDfltStylePos + nIndex );
        m_pLbFormat->SelectEntryPos( nDfltStylePos + nIndex-1 );

        pTableTable->EraseAutoFormat(nIndex);
        nIndex--;

        if( !nIndex )
        {
            m_pBtnRemove->Enable(false);
            m_pBtnRename->Enable(false);
        }

        if( !bCoreDataChanged )
        {
            m_pBtnCancel->SetText( aStrClose );
            bCoreDataChanged = true;
        }
    }

    SelFormatHdl( *m_pLbFormat );
}

IMPL_LINK_NOARG(SwAutoFormatDlg, RenameHdl, Button*, void)
{
    bool bOk = false;
    while( !bOk )
    {
        SwStringInputDlg aDlg(GetFrameWeld(), aStrRenameTitle, aStrLabel, m_pLbFormat->GetSelectedEntry());
        if (aDlg.run() == RET_OK)
        {
            bool bFormatRenamed = false;
            const OUString aFormatName(aDlg.GetInputString());

            if ( !aFormatName.isEmpty() )
            {
                size_t n;
                for( n = 0; n < pTableTable->size(); ++n )
                    if ((*pTableTable)[n].GetName() == aFormatName)
                        break;

                if( n >= pTableTable->size() )
                {
                    // no format with this name exists, so rename it
                    m_pLbFormat->RemoveEntry( nDfltStylePos + nIndex );
                    std::unique_ptr<SwTableAutoFormat> p(
                            pTableTable->ReleaseAutoFormat(nIndex));

                    p->SetName( aFormatName );

                    // keep all arrays sorted!
                    for( n = 1; n < pTableTable->size(); ++n )
                        if ((*pTableTable)[n].GetName() > aFormatName)
                        {
                            break;
                        }

                    pTableTable->InsertAutoFormat( n, std::move(p) );
                    m_pLbFormat->InsertEntry( aFormatName, nDfltStylePos + n );
                    m_pLbFormat->SelectEntryPos( nDfltStylePos + n );

                    if ( !bCoreDataChanged )
                    {
                        m_pBtnCancel->SetText( aStrClose );
                        bCoreDataChanged = true;
                    }

                    SelFormatHdl( *m_pLbFormat );
                    bOk = true;
                    bFormatRenamed = true;
                }
            }

            if( !bFormatRenamed )
            {
                std::unique_ptr<weld::MessageDialog> xBox(Application::CreateMessageDialog(GetFrameWeld(), VclMessageType::Error, VclButtonsType::OkCancel, aStrInvalidFormat));
                bOk = RET_CANCEL == xBox->run();
            }
        }
        else
            bOk = true;
    }
}

IMPL_LINK_NOARG(SwAutoFormatDlg, SelFormatHdl, ListBox&, void)
{
    bool bBtnEnable = false;
    sal_uInt8 nSelPos = static_cast<sal_uInt8>(m_pLbFormat->GetSelectedEntryPos()), nOldIdx = nIndex;
    if( nSelPos >= nDfltStylePos )
    {
        nIndex = nSelPos - nDfltStylePos;
        m_pWndPreview->NotifyChange( (*pTableTable)[nIndex] );
        bBtnEnable = 0 != nIndex;
        UpdateChecks( (*pTableTable)[nIndex], true );
    }
    else
    {
        nIndex = 255;

        SwTableAutoFormat aTmp( SwViewShell::GetShellRes()->aStrNone );
        lcl_SetProperties( &aTmp, false );

        if( nOldIdx != nIndex )
            m_pWndPreview->NotifyChange( aTmp );
        UpdateChecks( aTmp, false );
    }

    m_pBtnRemove->Enable( bBtnEnable );
    m_pBtnRename->Enable( bBtnEnable );
}

IMPL_LINK_NOARG(SwAutoFormatDlg, OkHdl, Button*, void)
{
    if( bSetAutoFormat )
        pShell->SetTableStyle((*pTableTable)[nIndex]);
    EndDialog( RET_OK );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
