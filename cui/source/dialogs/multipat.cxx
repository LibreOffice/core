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
#include <vcl/msgbox.hxx>
#include <sfx2/filedlghelper.hxx>

#include "multipat.hxx"
#include <dialmgr.hxx>

#include <cuires.hrc>
#include <comphelper/processfactory.hxx>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/ui/dialogs/FolderPicker.hpp>
#include <com/sun/star/ui/dialogs/ExecutableDialogResults.hpp>

#include <unotools/pathoptions.hxx>
#include "svtools/treelistentry.hxx"

using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::ui::dialogs;
using namespace ::com::sun::star::uno;

IMPL_LINK_NOARG_TYPED(SvxMultiPathDialog, SelectHdl_Impl, SvTreeListBox*, void)
{
    sal_uLong nCount = m_pRadioLB->GetEntryCount();
    bool bIsSelected = m_pRadioLB->FirstSelected() != nullptr;
    bool bEnable = nCount > 1;
    m_pDelBtn->Enable(bEnable && bIsSelected);
}

IMPL_LINK_NOARG_TYPED(SvxPathSelectDialog, SelectHdl_Impl, ListBox&, void)
{
    sal_uLong nCount = m_pPathLB->GetEntryCount();
    bool bIsSelected = m_pPathLB->GetSelectEntryPos() != LISTBOX_ENTRY_NOTFOUND;
    bool bEnable = nCount > 1;
    m_pDelBtn->Enable(bEnable && bIsSelected);
}

IMPL_LINK_TYPED( SvxMultiPathDialog, CheckHdl_Impl, SvTreeListBox*, pBox, void )
{
    SvTreeListEntry* pEntry =
        pBox
        ? pBox->GetEntry( static_cast<svx::SvxRadioButtonListBox*>(pBox)->GetCurMousePoint() )
        : m_pRadioLB->FirstSelected();
    if ( pEntry )
        m_pRadioLB->HandleEntryChecked( pEntry );
}

IMPL_LINK_NOARG_TYPED(SvxMultiPathDialog, AddHdl_Impl, Button*, void)
{
    Reference < XComponentContext > xContext( ::comphelper::getProcessComponentContext() );
    Reference < XFolderPicker2 >  xFolderPicker = FolderPicker::create(xContext);

    if ( xFolderPicker->execute() == ExecutableDialogResults::OK )
    {
        INetURLObject aPath( xFolderPicker->getDirectory() );
        aPath.removeFinalSlash();
        OUString aURL = aPath.GetMainURL( INetURLObject::NO_DECODE );
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
            OUString sMsg( CUI_RES( RID_MULTIPATH_DBL_ERR ) );
            sMsg = sMsg.replaceFirst( "%1", sInsPath );
            ScopedVclPtrInstance<InfoBox>(this, sMsg)->Execute();
        }

        SelectHdl_Impl( nullptr );
    }
}

IMPL_LINK_NOARG_TYPED(SvxPathSelectDialog, AddHdl_Impl, Button*, void)
{
    Reference < XComponentContext > xContext( ::comphelper::getProcessComponentContext() );
    Reference < XFolderPicker2 >  xFolderPicker = FolderPicker::create(xContext);

    if ( xFolderPicker->execute() == ExecutableDialogResults::OK )
    {
        INetURLObject aPath( xFolderPicker->getDirectory() );
        aPath.removeFinalSlash();
        OUString aURL = aPath.GetMainURL( INetURLObject::NO_DECODE );
        OUString sInsPath;
        osl::FileBase::getSystemPathFromFileURL(aURL, sInsPath);

        if ( LISTBOX_ENTRY_NOTFOUND != m_pPathLB->GetEntryPos( sInsPath ) )
        {
            OUString sMsg( CUI_RES( RID_MULTIPATH_DBL_ERR ) );
            sMsg = sMsg.replaceFirst( "%1", sInsPath );
            ScopedVclPtrInstance<InfoBox>(this, sMsg)->Execute();
        }
        else
        {
            const sal_Int32 nPos = m_pPathLB->InsertEntry( sInsPath );
            m_pPathLB->SetEntryData( nPos, new OUString( aURL ) );
        }

        SelectHdl_Impl( *m_pPathLB );
    }
}

IMPL_LINK_NOARG_TYPED(SvxMultiPathDialog, DelHdl_Impl, Button*, void)
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

IMPL_LINK_NOARG_TYPED(SvxPathSelectDialog, DelHdl_Impl, Button*, void)
{
    sal_Int32 nPos = m_pPathLB->GetSelectEntryPos();
    m_pPathLB->RemoveEntry( nPos );
    sal_Int32 nCnt = m_pPathLB->GetEntryCount();

    if ( nCnt )
    {
        nCnt--;

        if ( nPos > nCnt )
            nPos = nCnt;
        m_pPathLB->SelectEntryPos( nPos );
    }

    SelectHdl_Impl( *m_pPathLB );
}

SvxMultiPathDialog::SvxMultiPathDialog(vcl::Window* pParent)
    : ModalDialog(pParent, "MultiPathDialog", "cui/ui/multipathdialog.ui")
{
    get(m_pAddBtn, "add");
    get(m_pDelBtn, "delete");

    SvSimpleTableContainer* pRadioLBContainer = get<SvSimpleTableContainer>("paths");
    Size aSize(LogicToPixel(Size(195, 77), MAP_APPFONT));
    pRadioLBContainer->set_width_request(aSize.Width());
    pRadioLBContainer->set_height_request(aSize.Height());
    m_pRadioLB = VclPtr<svx::SvxRadioButtonListBox>::Create(*pRadioLBContainer, 0);

    static long aStaticTabs[]= { 2, 0, 12 };
    m_pRadioLB->SvSimpleTable::SetTabs( aStaticTabs );
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

SvxPathSelectDialog::SvxPathSelectDialog(vcl::Window* pParent)
    : ModalDialog(pParent, "SelectPathDialog", "cui/ui/selectpathdialog.ui")
{
    get(m_pAddBtn, "add");
    get(m_pDelBtn, "delete");
    get(m_pPathLB, "paths");
    Size aSize(LogicToPixel(Size(189, 80), MAP_APPFONT));
    m_pPathLB->set_width_request(aSize.Width());
    m_pPathLB->set_height_request(aSize.Height());

    m_pPathLB->SetSelectHdl( LINK( this, SvxPathSelectDialog, SelectHdl_Impl ) );
    m_pAddBtn->SetClickHdl( LINK( this, SvxPathSelectDialog, AddHdl_Impl ) );
    m_pDelBtn->SetClickHdl( LINK( this, SvxPathSelectDialog, DelHdl_Impl ) );

    SelectHdl_Impl( *m_pPathLB );
}

SvxMultiPathDialog::~SvxMultiPathDialog()
{
    disposeOnce();
}

void SvxMultiPathDialog::dispose()
{
    if (m_pRadioLB)
    {
        sal_uInt16 nPos = (sal_uInt16)m_pRadioLB->GetEntryCount();
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

SvxPathSelectDialog::~SvxPathSelectDialog()
{
    disposeOnce();
}

void SvxPathSelectDialog::dispose()
{
    if (m_pPathLB)
    {
        sal_Int32 nPos = m_pPathLB->GetEntryCount();
        while ( nPos-- )
            delete static_cast<OUString*>(m_pPathLB->GetEntryData(nPos));
    }
    m_pPathLB.clear();
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
                sNewPath += OUString(cDelim);
            sNewPath += *static_cast<OUString*>(pEntry->GetUserData());
        }
    }
    if ( !sNewPath.isEmpty() )
        sNewPath += OUString(cDelim);
    sNewPath += sWritable;

    return sNewPath;
}

OUString SvxPathSelectDialog::GetPath() const
{
    OUString sNewPath;
    sal_Unicode cDelim = SVT_SEARCHPATH_DELIMITER;

    for ( sal_Int32 i = 0; i < m_pPathLB->GetEntryCount(); ++i )
    {
        if ( !sNewPath.isEmpty() )
            sNewPath += OUString(cDelim);
        sNewPath += *static_cast<OUString*>(m_pPathLB->GetEntryData(i));
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
    sal_Unicode cDelim = SVT_SEARCHPATH_DELIMITER;

    if ( !rPath.isEmpty() )
    {
        sal_Int32 nIndex = 0;
        do
        {
            const OUString sPath = rPath.getToken( 0, cDelim, nIndex );
            OUString sSystemPath;
            bool bIsSystemPath =
                osl::FileBase::getSystemPathFromFileURL(sPath, sSystemPath) == osl::FileBase::E_None;

            const sal_Int32 nPos = m_pPathLB->InsertEntry( bIsSystemPath ? sSystemPath : sPath );
            m_pPathLB->SetEntryData( nPos, new OUString( sPath ) );
        }
        while (nIndex >= 0);
    }

    SelectHdl_Impl( *m_pPathLB );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
