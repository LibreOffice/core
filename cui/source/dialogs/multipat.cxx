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

#include <tools/urlobj.hxx>
#include <vcl/msgbox.hxx>
#include <sfx2/filedlghelper.hxx>

#include "multipat.hxx"
#include <dialmgr.hxx>

#include <cuires.hrc>
#include <comphelper/processfactory.hxx>
#include <comphelper/string.hxx>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/ui/dialogs/FolderPicker.hpp>
#include <com/sun/star/ui/dialogs/ExecutableDialogResults.hpp>

#include <unotools/localfilehelper.hxx>
#include <unotools/pathoptions.hxx>
#include "svtools/treelistentry.hxx"

using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::ui::dialogs;
using namespace ::com::sun::star::uno;

IMPL_LINK_NOARG(SvxMultiPathDialog, SelectHdl_Impl)
{
    sal_uLong nCount = m_pRadioLB->GetEntryCount();
    bool bIsSelected = m_pRadioLB->FirstSelected() != NULL;
    bool bEnable = nCount > 1;
    m_pDelBtn->Enable(bEnable && bIsSelected);
    return 0;
}

IMPL_LINK_NOARG(SvxPathSelectDialog, SelectHdl_Impl)
{
    sal_uLong nCount = m_pPathLB->GetEntryCount();
    bool bIsSelected = m_pPathLB->GetSelectEntryPos() != LISTBOX_ENTRY_NOTFOUND;
    bool bEnable = nCount > 1;
    m_pDelBtn->Enable(bEnable && bIsSelected);
    return 0;
}

IMPL_LINK( SvxMultiPathDialog, CheckHdl_Impl, svx::SvxRadioButtonListBox *, pBox )
{
    SvTreeListEntry* pEntry =
        pBox ? pBox->GetEntry( pBox->GetCurMousePoint() ) : m_pRadioLB->FirstSelected();
    if ( pEntry )
        m_pRadioLB->HandleEntryChecked( pEntry );
    return 0;
}

IMPL_LINK_NOARG(SvxMultiPathDialog, AddHdl_Impl)
{
    Reference < XComponentContext > xContext( ::comphelper::getProcessComponentContext() );
    Reference < XFolderPicker2 >  xFolderPicker = FolderPicker::create(xContext);

    if ( xFolderPicker->execute() == ExecutableDialogResults::OK )
    {
        INetURLObject aPath( xFolderPicker->getDirectory() );
        aPath.removeFinalSlash();
        OUString aURL = aPath.GetMainURL( INetURLObject::NO_DECODE );
        OUString sInsPath;
        ::utl::LocalFileHelper::ConvertURLToSystemPath( aURL, sInsPath );

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
            InfoBox( this, sMsg ).Execute();
        }

        SelectHdl_Impl( NULL );
    }
    return 0;
}

IMPL_LINK_NOARG(SvxPathSelectDialog, AddHdl_Impl)
{
    Reference < XComponentContext > xContext( ::comphelper::getProcessComponentContext() );
    Reference < XFolderPicker2 >  xFolderPicker = FolderPicker::create(xContext);

    if ( xFolderPicker->execute() == ExecutableDialogResults::OK )
    {
        INetURLObject aPath( xFolderPicker->getDirectory() );
        aPath.removeFinalSlash();
        OUString aURL = aPath.GetMainURL( INetURLObject::NO_DECODE );
        OUString sInsPath;
        ::utl::LocalFileHelper::ConvertURLToSystemPath( aURL, sInsPath );

        if ( LISTBOX_ENTRY_NOTFOUND != m_pPathLB->GetEntryPos( sInsPath ) )
        {
            OUString sMsg( CUI_RES( RID_MULTIPATH_DBL_ERR ) );
            sMsg = sMsg.replaceFirst( "%1", sInsPath );
            InfoBox( this, sMsg ).Execute();
        }
        else
        {
            sal_uInt16 nPos = m_pPathLB->InsertEntry( sInsPath, LISTBOX_APPEND );
            m_pPathLB->SetEntryData( nPos, new OUString( aURL ) );
        }

        SelectHdl_Impl( NULL );
    }
    return 0;
}

IMPL_LINK_NOARG(SvxMultiPathDialog, DelHdl_Impl)
{
    SvTreeListEntry* pEntry = m_pRadioLB->FirstSelected();
    delete (OUString*)pEntry->GetUserData();
    bool bChecked = m_pRadioLB->GetCheckButtonState( pEntry ) == SV_BUTTON_CHECKED;
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
            m_pRadioLB->SetCheckButtonState( pEntry, SV_BUTTON_CHECKED );
            m_pRadioLB->HandleEntryChecked( pEntry );
        }
        else
            m_pRadioLB->Select( pEntry );
    }

    SelectHdl_Impl( NULL );
    return 0;
}

IMPL_LINK_NOARG(SvxPathSelectDialog, DelHdl_Impl)
{
    sal_uInt16 nPos = m_pPathLB->GetSelectEntryPos();
    m_pPathLB->RemoveEntry( nPos );
    sal_uInt16 nCnt = m_pPathLB->GetEntryCount();

    if ( nCnt )
    {
        nCnt--;

        if ( nPos > nCnt )
            nPos = nCnt;
        m_pPathLB->SelectEntryPos( nPos );
    }

    SelectHdl_Impl( NULL );
    return 0;
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
    m_pRadioLB = new svx::SvxRadioButtonListBox(*pRadioLBContainer, 0);

    static long aStaticTabs[]= { 2, 0, 12 };
    m_pRadioLB->SvSimpleTable::SetTabs( aStaticTabs );
    OUString sHeader(get<FixedText>("pathlist")->GetText());
    m_pRadioLB->SetQuickHelpText( sHeader );
    sHeader = "\t" + sHeader;
    m_pRadioLB->InsertHeaderEntry( sHeader, HEADERBAR_APPEND, HIB_LEFT );

    m_pRadioLB->SetSelectHdl( LINK( this, SvxMultiPathDialog, SelectHdl_Impl ) );
    m_pRadioLB->SetCheckButtonHdl( LINK( this, SvxMultiPathDialog, CheckHdl_Impl ) );
    m_pAddBtn->SetClickHdl( LINK( this, SvxMultiPathDialog, AddHdl_Impl ) );
    m_pDelBtn->SetClickHdl( LINK( this, SvxMultiPathDialog, DelHdl_Impl ) );

    SelectHdl_Impl( NULL );

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

    SelectHdl_Impl( NULL );
}

SvxMultiPathDialog::~SvxMultiPathDialog()
{
    sal_uInt16 nPos = (sal_uInt16)m_pRadioLB->GetEntryCount();
    while ( nPos-- )
    {
        SvTreeListEntry* pEntry = m_pRadioLB->GetEntry( nPos );
        delete (OUString*)pEntry->GetUserData();
    }

    delete m_pRadioLB;
}

SvxPathSelectDialog::~SvxPathSelectDialog()
{
    sal_uInt16 nPos = m_pPathLB->GetEntryCount();
    while ( nPos-- )
        delete (OUString*)m_pPathLB->GetEntryData(nPos);
}

OUString SvxMultiPathDialog::GetPath() const
{
    OUString sNewPath;
    sal_Unicode cDelim = SVT_SEARCHPATH_DELIMITER;

    OUString sWritable;
    for ( sal_uInt16 i = 0; i < m_pRadioLB->GetEntryCount(); ++i )
    {
        SvTreeListEntry* pEntry = m_pRadioLB->GetEntry(i);
        if ( m_pRadioLB->GetCheckButtonState( pEntry ) == SV_BUTTON_CHECKED )
            sWritable = *(OUString*)pEntry->GetUserData();
        else
        {
            if ( !sNewPath.isEmpty() )
                sNewPath += OUString(cDelim);
            sNewPath += *(OUString*)pEntry->GetUserData();
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

    for ( sal_uInt16 i = 0; i < m_pPathLB->GetEntryCount(); ++i )
    {
        if ( !sNewPath.isEmpty() )
            sNewPath += OUString(cDelim);
        sNewPath += *(OUString*)m_pPathLB->GetEntryData(i);
    }

    return sNewPath;
}

void SvxMultiPathDialog::SetPath( const OUString& rPath )
{
    sal_Unicode cDelim = SVT_SEARCHPATH_DELIMITER;
    sal_uInt16 nCount = comphelper::string::getTokenCount(rPath, cDelim);

    for ( sal_uInt16 i = 0; i < nCount; ++i )
    {
        OUString sPath = rPath.getToken( i, cDelim );
        OUString sSystemPath;
        bool bIsSystemPath =
            ::utl::LocalFileHelper::ConvertURLToSystemPath( sPath, sSystemPath );

        OUString sEntry( '\t' );
        sEntry += (bIsSystemPath ? sSystemPath : OUString(sPath));
        SvTreeListEntry* pEntry = m_pRadioLB->InsertEntry( sEntry );
        OUString* pURL = new OUString( sPath );
        pEntry->SetUserData( pURL );
    }

    if (nCount > 0)
    {
        SvTreeListEntry* pEntry = m_pRadioLB->GetEntry( nCount - 1 );
        if ( pEntry )
        {
            m_pRadioLB->SetCheckButtonState( pEntry, SV_BUTTON_CHECKED );
            m_pRadioLB->HandleEntryChecked( pEntry );
        }
    }

    SelectHdl_Impl( NULL );
}

void SvxPathSelectDialog::SetPath(const OUString& rPath)
{
    sal_Unicode cDelim = SVT_SEARCHPATH_DELIMITER;
    sal_uInt16 nPos, nCount = comphelper::string::getTokenCount(rPath, cDelim);

    for ( sal_uInt16 i = 0; i < nCount; ++i )
    {
        OUString sPath = rPath.getToken( i, cDelim );
        OUString sSystemPath;
        bool bIsSystemPath =
            ::utl::LocalFileHelper::ConvertURLToSystemPath( sPath, sSystemPath );

        if ( bIsSystemPath )
            nPos = m_pPathLB->InsertEntry( sSystemPath, LISTBOX_APPEND );
        else
            nPos = m_pPathLB->InsertEntry( sPath, LISTBOX_APPEND );
        m_pPathLB->SetEntryData( nPos, new OUString( sPath ) );
    }

    SelectHdl_Impl( NULL );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
