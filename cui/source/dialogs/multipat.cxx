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

#include <sal/config.h>

#include <osl/file.hxx>
#include <tools/urlobj.hxx>
#include <vcl/svapp.hxx>
#include <vcl/weld.hxx>
#include <sfx2/filedlghelper.hxx>

#include <multipat.hxx>
#include <dialmgr.hxx>

#include <strings.hrc>
#include <comphelper/processfactory.hxx>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/ui/dialogs/FolderPicker.hpp>
#include <com/sun/star/ui/dialogs/ExecutableDialogResults.hpp>

#include <unotools/pathoptions.hxx>
#include <svtools/treelistentry.hxx>

using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::ui::dialogs;
using namespace ::com::sun::star::uno;

IMPL_LINK_NOARG(SvxMultiPathDialog, SelectHdl_Impl, SvTreeListBox*, void)
{
    sal_uLong nCount = m_pRadioLB->GetEntryCount();
    bool bIsSelected = m_pRadioLB->FirstSelected() != nullptr;
    bool bEnable = nCount > 1;
    m_pDelBtn->Enable(bEnable && bIsSelected);
}

IMPL_LINK_NOARG(SvxPathSelectDialog, SelectHdl_Impl, weld::TreeView&, void)
{
    sal_uLong nCount = m_xPathLB->n_children();
    bool bIsSelected = m_xPathLB->get_selected_index() != -1;
    bool bEnable = nCount > 1;
    m_xDelBtn->set_sensitive(bEnable && bIsSelected);
}

IMPL_LINK( SvxMultiPathDialog, CheckHdl_Impl, SvTreeListBox*, pBox, void )
{
    SvTreeListEntry* pEntry =
        pBox
        ? pBox->GetEntry( static_cast<svx::SvxRadioButtonListBox*>(pBox)->GetCurMousePoint() )
        : m_pRadioLB->FirstSelected();
    if ( pEntry )
        m_pRadioLB->HandleEntryChecked( pEntry );
}

IMPL_LINK_NOARG(SvxMultiPathDialog, AddHdl_Impl, Button*, void)
{
    Reference < XComponentContext > xContext( ::comphelper::getProcessComponentContext() );
    Reference < XFolderPicker2 >  xFolderPicker = FolderPicker::create(xContext);

    if ( xFolderPicker->execute() == ExecutableDialogResults::OK )
    {
        INetURLObject aPath( xFolderPicker->getDirectory() );
        aPath.removeFinalSlash();
        OUString aURL = aPath.GetMainURL( INetURLObject::DecodeMechanism::NONE );
        OUString sInsPath;
        osl::FileBase::getSystemPathFromFileURL(aURL, sInsPath);

        sal_uLong nPos = m_pRadioLB->GetEntryPos( sInsPath, 1 );
        if ( 0xffffffff == nPos ) //See svtools/source/contnr/svtabbx.cxx SvTabListBox::GetEntryPos
        {
            OUString sNewEntry( '\t' );
            sNewEntry += sInsPath;
            SvTreeListEntry* pEntry = m_pRadioLB->InsertEntry( sNewEntry );
            OUString* pData = new OUString( aURL );
            pEntry->SetUserData( pData );
        }
        else
        {
            OUString sMsg( CuiResId( RID_MULTIPATH_DBL_ERR ) );
            sMsg = sMsg.replaceFirst( "%1", sInsPath );
            std::unique_ptr<weld::MessageDialog> xInfoBox(Application::CreateMessageDialog(GetFrameWeld(),
                                                          VclMessageType::Info, VclButtonsType::Ok, sMsg));
            xInfoBox->run();
        }

        SelectHdl_Impl( nullptr );
    }
}

IMPL_LINK_NOARG(SvxPathSelectDialog, AddHdl_Impl, weld::Button&, void)
{
    Reference < XComponentContext > xContext( ::comphelper::getProcessComponentContext() );
    Reference < XFolderPicker2 >  xFolderPicker = FolderPicker::create(xContext);

    if ( xFolderPicker->execute() == ExecutableDialogResults::OK )
    {
        INetURLObject aPath( xFolderPicker->getDirectory() );
        aPath.removeFinalSlash();
        OUString aURL = aPath.GetMainURL( INetURLObject::DecodeMechanism::NONE );
        OUString sInsPath;
        osl::FileBase::getSystemPathFromFileURL(aURL, sInsPath);

        if (m_xPathLB->find_text(sInsPath) != -1)
        {
            OUString sMsg( CuiResId( RID_MULTIPATH_DBL_ERR ) );
            sMsg = sMsg.replaceFirst( "%1", sInsPath );
            std::unique_ptr<weld::MessageDialog> xInfoBox(Application::CreateMessageDialog(m_xDialog.get(),
                                                          VclMessageType::Info, VclButtonsType::Ok, sMsg));
            xInfoBox->run();
        }
        else
        {
            m_xPathLB->append(aURL, sInsPath, "");
        }

        SelectHdl_Impl(*m_xPathLB);
    }
}

IMPL_LINK_NOARG(SvxMultiPathDialog, DelHdl_Impl, Button*, void)
{
    SvTreeListEntry* pEntry = m_pRadioLB->FirstSelected();
    delete static_cast<OUString*>(pEntry->GetUserData());
    bool bChecked = m_pRadioLB->GetCheckButtonState( pEntry ) == SvButtonState::Checked;
    sal_uLong nPos = m_pRadioLB->GetEntryPos( pEntry );
    m_pRadioLB->RemoveEntry( pEntry );
    sal_uLong nCnt = m_pRadioLB->GetEntryCount();
    if ( nCnt )
    {
        nCnt--;
        if ( nPos > nCnt )
            nPos = nCnt;
        pEntry = m_pRadioLB->GetEntry( nPos );
        if ( bChecked )
        {
            m_pRadioLB->SetCheckButtonState( pEntry, SvButtonState::Checked );
            m_pRadioLB->HandleEntryChecked( pEntry );
        }
        else
            m_pRadioLB->Select( pEntry );
    }

    SelectHdl_Impl( nullptr );
}

IMPL_LINK_NOARG(SvxPathSelectDialog, DelHdl_Impl, weld::Button&, void)
{
    int nPos = m_xPathLB->get_selected_index();
    m_xPathLB->remove(nPos);
    int nCnt = m_xPathLB->n_children();

    if (nCnt)
    {
        --nCnt;

        if ( nPos > nCnt )
            nPos = nCnt;
        m_xPathLB->select(nPos);
    }

    SelectHdl_Impl(*m_xPathLB);
}

SvxMultiPathDialog::SvxMultiPathDialog(vcl::Window* pParent)
    : ModalDialog(pParent, "MultiPathDialog", "cui/ui/multipathdialog.ui")
{
    get(m_pAddBtn, "add");
    get(m_pDelBtn, "delete");

    SvSimpleTableContainer* pRadioLBContainer = get<SvSimpleTableContainer>("paths");
    Size aSize(LogicToPixel(Size(195, 77), MapMode(MapUnit::MapAppFont)));
    pRadioLBContainer->set_width_request(aSize.Width());
    pRadioLBContainer->set_height_request(aSize.Height());
    m_pRadioLB = VclPtr<svx::SvxRadioButtonListBox>::Create(*pRadioLBContainer, 0);

    static long aStaticTabs[]= { 0, 12 };
    m_pRadioLB->SvSimpleTable::SetTabs( SAL_N_ELEMENTS(aStaticTabs), aStaticTabs );
    OUString sHeader(get<FixedText>("pathlist")->GetText());
    m_pRadioLB->SetQuickHelpText( sHeader );
    sHeader = "\t" + sHeader;
    m_pRadioLB->InsertHeaderEntry( sHeader, HEADERBAR_APPEND, HeaderBarItemBits::LEFT );

    m_pRadioLB->SetSelectHdl( LINK( this, SvxMultiPathDialog, SelectHdl_Impl ) );
    m_pRadioLB->SetCheckButtonHdl( LINK( this, SvxMultiPathDialog, CheckHdl_Impl ) );
    m_pAddBtn->SetClickHdl( LINK( this, SvxMultiPathDialog, AddHdl_Impl ) );
    m_pDelBtn->SetClickHdl( LINK( this, SvxMultiPathDialog, DelHdl_Impl ) );

    SelectHdl_Impl( nullptr );

    m_pRadioLB->ShowTable();
}

SvxPathSelectDialog::SvxPathSelectDialog(weld::Window* pParent)
    : GenericDialogController(pParent, "cui/ui/selectpathdialog.ui", "SelectPathDialog")
    , m_xPathLB(m_xBuilder->weld_tree_view("paths"))
    , m_xAddBtn(m_xBuilder->weld_button("add"))
    , m_xDelBtn(m_xBuilder->weld_button("delete"))
{
    m_xPathLB->set_size_request(m_xPathLB->get_approximate_digit_width() * 60,
                                m_xPathLB->get_text_height() * 10);

    m_xPathLB->connect_changed(LINK(this, SvxPathSelectDialog, SelectHdl_Impl));
    m_xAddBtn->connect_clicked(LINK(this, SvxPathSelectDialog, AddHdl_Impl));
    m_xDelBtn->connect_clicked(LINK(this, SvxPathSelectDialog, DelHdl_Impl));

    SelectHdl_Impl(*m_xPathLB);
}

SvxMultiPathDialog::~SvxMultiPathDialog()
{
    disposeOnce();
}

void SvxMultiPathDialog::dispose()
{
    if (m_pRadioLB)
    {
        sal_uInt16 nPos = static_cast<sal_uInt16>(m_pRadioLB->GetEntryCount());
        while ( nPos-- )
        {
            SvTreeListEntry* pEntry = m_pRadioLB->GetEntry( nPos );
            delete static_cast<OUString*>(pEntry->GetUserData());
        }
    }

    m_pRadioLB.disposeAndClear();
    m_pAddBtn.clear();
    m_pDelBtn.clear();
    ModalDialog::dispose();
}

OUString SvxMultiPathDialog::GetPath() const
{
    OUString sNewPath;
    sal_Unicode cDelim = SVT_SEARCHPATH_DELIMITER;

    OUString sWritable;
    for ( sal_uLong i = 0; i < m_pRadioLB->GetEntryCount(); ++i )
    {
        SvTreeListEntry* pEntry = m_pRadioLB->GetEntry(i);
        if ( m_pRadioLB->GetCheckButtonState( pEntry ) == SvButtonState::Checked )
            sWritable = *static_cast<OUString*>(pEntry->GetUserData());
        else
        {
            if ( !sNewPath.isEmpty() )
                sNewPath += OUStringLiteral1(cDelim);
            sNewPath += *static_cast<OUString*>(pEntry->GetUserData());
        }
    }
    if ( !sNewPath.isEmpty() )
        sNewPath += OUStringLiteral1(cDelim);
    sNewPath += sWritable;

    return sNewPath;
}

OUString SvxPathSelectDialog::GetPath() const
{
    OUString sNewPath;

    for (int i = 0; i < m_xPathLB->n_children(); ++i)
    {
        if ( !sNewPath.isEmpty() )
            sNewPath += OUStringLiteral1(SVT_SEARCHPATH_DELIMITER);
        sNewPath += m_xPathLB->get_id(i);
    }

    return sNewPath;
}

void SvxMultiPathDialog::SetPath( const OUString& rPath )
{
    if ( !rPath.isEmpty() )
    {
        const sal_Unicode cDelim = SVT_SEARCHPATH_DELIMITER;
        sal_uLong nCount = 0;
        sal_Int32 nIndex = 0;
        do
        {
            const OUString sPath = rPath.getToken( 0, cDelim, nIndex );
            OUString sSystemPath;
            bool bIsSystemPath =
                osl::FileBase::getSystemPathFromFileURL(sPath, sSystemPath) == osl::FileBase::E_None;

            const OUString sEntry( "\t" + (bIsSystemPath ? sSystemPath : sPath));
            SvTreeListEntry* pEntry = m_pRadioLB->InsertEntry( sEntry );
            OUString* pURL = new OUString( sPath );
            pEntry->SetUserData( pURL );
            ++nCount;
        }
        while (nIndex >= 0);

        SvTreeListEntry* pEntry = m_pRadioLB->GetEntry( nCount - 1 );
        if ( pEntry )
        {
            m_pRadioLB->SetCheckButtonState( pEntry, SvButtonState::Checked );
            m_pRadioLB->HandleEntryChecked( pEntry );
        }
    }

    SelectHdl_Impl( nullptr );
}

void SvxPathSelectDialog::SetPath(const OUString& rPath)
{
    if ( !rPath.isEmpty() )
    {
        sal_Int32 nIndex = 0;
        do
        {
            const OUString sPath = rPath.getToken( 0, SVT_SEARCHPATH_DELIMITER, nIndex );
            OUString sSystemPath;
            bool bIsSystemPath =
                osl::FileBase::getSystemPathFromFileURL(sPath, sSystemPath) == osl::FileBase::E_None;

            m_xPathLB->append(sPath, bIsSystemPath ? sSystemPath : sPath, "");
        }
        while (nIndex >= 0);
    }

    SelectHdl_Impl(*m_xPathLB);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
