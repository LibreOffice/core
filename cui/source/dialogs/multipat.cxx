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
#include <vcl/treelistentry.hxx>

using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::ui::dialogs;
using namespace ::com::sun::star::uno;

IMPL_LINK_NOARG(SvxMultiPathDialog, SelectHdl_Impl, weld::TreeView&, void)
{
    auto nCount = m_xRadioLB->n_children();
    bool bIsSelected = m_xRadioLB->get_selected_index() != -1;
    bool bEnable = nCount > 1;
    m_xDelBtn->set_sensitive(bEnable && bIsSelected);
}

IMPL_LINK_NOARG(SvxPathSelectDialog, SelectHdl_Impl, weld::TreeView&, void)
{
    auto nCount = m_xPathLB->n_children();
    bool bIsSelected = m_xPathLB->get_selected_index() != -1;
    bool bEnable = nCount > 1;
    m_xDelBtn->set_sensitive(bEnable && bIsSelected);
}

void SvxMultiPathDialog::HandleEntryChecked(int nRow)
{
    m_xRadioLB->select(nRow);
    bool bChecked = m_xRadioLB->get_toggle(nRow, 0);
    if (bChecked)
    {
        // we have radio button behavior -> so uncheck the other entries
        int nCount = m_xRadioLB->n_children();
        for (int i = 0; i < nCount; ++i)
        {
            if (i != nRow)
                m_xRadioLB->set_toggle(i, false, 0);
        }
    }
}

IMPL_LINK(SvxMultiPathDialog, CheckHdl_Impl, const row_col&, rRowCol, void)
{
    HandleEntryChecked(rRowCol.first);
}

void SvxMultiPathDialog::AppendEntry(const OUString& rText, const OUString& rId)
{
    m_xRadioLB->append();
    const int nRow = m_xRadioLB->n_children() - 1;
    m_xRadioLB->set_toggle(nRow, false, 0);
    m_xRadioLB->set_text(nRow, rText, 1);
    m_xRadioLB->set_id(nRow, rId);
}

IMPL_LINK_NOARG(SvxMultiPathDialog, AddHdl_Impl, weld::Button&, void)
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

        if (m_xRadioLB->find_text(sInsPath) != -1)
        {
            OUString sMsg( CuiResId( RID_MULTIPATH_DBL_ERR ) );
            sMsg = sMsg.replaceFirst( "%1", sInsPath );
            std::unique_ptr<weld::MessageDialog> xInfoBox(Application::CreateMessageDialog(m_xDialog.get(),
                                                          VclMessageType::Info, VclButtonsType::Ok, sMsg));
            xInfoBox->run();
        }
        else
        {
            AppendEntry(sInsPath, aURL);
        }

        SelectHdl_Impl(*m_xRadioLB);
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
            m_xPathLB->append(aURL, sInsPath);
        }

        SelectHdl_Impl(*m_xPathLB);
    }
}

IMPL_LINK_NOARG(SvxMultiPathDialog, DelHdl_Impl, weld::Button&, void)
{
    int nPos = m_xRadioLB->get_selected_index();
    bool bChecked = m_xRadioLB->get_toggle(nPos, 0);
    m_xRadioLB->remove(nPos);
    int nCnt = m_xRadioLB->n_children();
    if (nCnt)
    {
        --nCnt;

        if ( nPos > nCnt )
            nPos = nCnt;
        if (bChecked)
        {
            m_xRadioLB->set_toggle(nPos, true, 0);
            HandleEntryChecked(nPos);
        }
        m_xRadioLB->select(nPos);
    }

    SelectHdl_Impl(*m_xRadioLB);
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

SvxMultiPathDialog::SvxMultiPathDialog(weld::Window* pParent)
    : GenericDialogController(pParent, "cui/ui/multipathdialog.ui", "MultiPathDialog")
    , m_xRadioLB(m_xBuilder->weld_tree_view("paths"))
    , m_xAddBtn(m_xBuilder->weld_button("add"))
    , m_xDelBtn(m_xBuilder->weld_button("delete"))
{
    m_xRadioLB->set_size_request(m_xRadioLB->get_approximate_digit_width() * 60,
                                 m_xRadioLB->get_text_height() * 10);

    std::vector<int> aWidths;
    aWidths.push_back(m_xRadioLB->get_checkbox_column_width());
    m_xRadioLB->set_column_fixed_widths(aWidths);

    std::vector<int> aRadioColumns;
    aRadioColumns.push_back(0);
    m_xRadioLB->set_toggle_columns_as_radio(aRadioColumns);
    m_xRadioLB->connect_toggled(LINK(this, SvxMultiPathDialog, CheckHdl_Impl));
    m_xRadioLB->connect_changed(LINK(this, SvxMultiPathDialog, SelectHdl_Impl));
    m_xAddBtn->connect_clicked(LINK(this, SvxMultiPathDialog, AddHdl_Impl));
    m_xDelBtn->connect_clicked(LINK(this, SvxMultiPathDialog, DelHdl_Impl));

    SelectHdl_Impl(*m_xRadioLB);
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
}

OUString SvxMultiPathDialog::GetPath() const
{
    OUStringBuffer sNewPath;
    sal_Unicode cDelim = SVT_SEARCHPATH_DELIMITER;

    OUString sWritable;
    for (int i = 0, nCount = m_xRadioLB->n_children(); i < nCount; ++i)
    {
        if (m_xRadioLB->get_toggle(i, 0))
            sWritable = m_xRadioLB->get_id(i);
        else
        {
            if (!sNewPath.isEmpty())
                sNewPath.append(cDelim);
            sNewPath.append(m_xRadioLB->get_id(i));
        }
    }
    if (!sNewPath.isEmpty())
        sNewPath.append(cDelim);
    sNewPath.append(sWritable);

    return sNewPath.makeStringAndClear();
}

OUString SvxPathSelectDialog::GetPath() const
{
    OUStringBuffer sNewPath;

    for (int i = 0; i < m_xPathLB->n_children(); ++i)
    {
        if ( !sNewPath.isEmpty() )
            sNewPath.append(SVT_SEARCHPATH_DELIMITER);
        sNewPath.append( m_xPathLB->get_id(i));
    }

    return sNewPath.makeStringAndClear();
}

void SvxMultiPathDialog::SetPath( const OUString& rPath )
{
    if ( !rPath.isEmpty() )
    {
        const sal_Unicode cDelim = SVT_SEARCHPATH_DELIMITER;
        int nCount = 0;
        sal_Int32 nIndex = 0;
        do
        {
            const OUString sPath = rPath.getToken( 0, cDelim, nIndex );
            OUString sSystemPath;
            bool bIsSystemPath =
                osl::FileBase::getSystemPathFromFileURL(sPath, sSystemPath) == osl::FileBase::E_None;

            const OUString sEntry((bIsSystemPath ? sSystemPath : sPath));
            AppendEntry(sEntry, sPath);
            ++nCount;
        }
        while (nIndex >= 0);

        if (nCount)
        {
            m_xRadioLB->set_toggle(nCount - 1, true, 0);
            HandleEntryChecked(nCount - 1);
        }
    }

    SelectHdl_Impl(*m_xRadioLB);
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

            m_xPathLB->append(sPath, bIsSystemPath ? sSystemPath : sPath);
        }
        while (nIndex >= 0);
    }

    SelectHdl_Impl(*m_xPathLB);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
